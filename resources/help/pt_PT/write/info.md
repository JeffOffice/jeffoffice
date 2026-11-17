# Info: propriedades, palavras-passe e Inspecionar

<!-- keywords: verificar acessibilidade, info, propriedades, título, autor, assunto, palavras-chave, categorias, comentários, empresa, gestor, propriedades personalizadas, docproperty, marcar como final, final, só de leitura, palavra-passe, cifrar, encriptar, proteger, palavra-passe de abertura, inspecionar documento, informações pessoais, texto oculto, xml personalizado, verificar compatibilidade, histórico de versões, versões, guardar uma versão, voltar atrás, repor, comparar versões, mostrar na pasta, copiar caminho, pasta, caminho -->

Clique em **Ficheiro > Info** para ver os dados do seu documento e para o proteger antes de o partilhar. Prima **Esc** para voltar ao documento.

## Onde está o ficheiro

No topo, a página indica onde o ficheiro está guardado. Clique em **Mostrar na Pasta** para abrir essa pasta. Clique em **Copiar Caminho** para copiar o caminho, de modo a poder colá-lo num correio eletrónico ou numa conversa.

## Propriedades

As propriedades são dados sobre o documento, como o título e quem o escreveu. A página tem uma caixa para cada uma: **Título**, **Autor**, **Assunto**, **Palavras-chave**, **Categorias**, **Comentários**, **Empresa** e **Gestor**. Escreva numa caixa e prima **Tab** ou **Enter**. O JeffWrite guarda a alteração e grava-a da próxima vez que guardar o documento.

Por baixo das caixas, a página mostra o que o JeffWrite calcula: quando o documento foi criado, guardado e impresso, e quantas páginas, palavras, carateres e parágrafos tem.

### As suas propriedades

Clique em **Propriedades Personalizadas** para criar propriedades próprias, como **Projeto** ou **Cliente**. Clique em **Adicionar**, escreva um nome, escolha o tipo de valor (texto, um número inteiro, um número, sim ou não, ou uma data) e escreva o valor.

Um campo pode mostrar uma propriedade personalizada. Clique em **Inserir > Campo** e escolha um campo. Altere o código para `DOCPROPERTY "Cliente"` e clique em **OK**. O texto mostra o valor. Prima **F9** para o atualizar depois de alterar a propriedade. Consulte [Campos](fields).

## Marcar como Final

Clique em **Marcar como Final** para indicar aos leitores que o documento está concluído. O JeffWrite guarda o documento e depois mantém-no apenas para leitura. Quando alguém o abre, a barra de título indica que é só de leitura, e uma nota informa que foi marcado como final. Trata-se de uma forma suave de dizer "por favor, não altere isto". Não é um bloqueio, porque qualquer pessoa pode escolher **Marcado como Final** novamente para editar o documento.

## Abrir Sempre Só para Leitura

Clique em **Abrir Sempre Só para Leitura** para que o documento seja aberto apenas para leitura de cada vez, evitando alterações acidentais. A opção fica guardada com o ficheiro quando o guardar. Da próxima vez que alguém abrir o ficheiro, a barra de estado indica que está aberto só para leitura. Para o editar, abra **Ficheiro > Info** e clique em **Abre Sempre Só para Leitura**. Basta um clique, por isso é um lembrete e não um bloqueio. Outros programas não leem esta opção, e um ficheiro .odt não a pode guardar; por isso, guarde primeiro o documento como ficheiro .docx. A **Verificação de Compatibilidade** indica-o para um ficheiro .odt.

## Cifrar com Palavra-passe

Clique em **Cifrar com Palavra-passe** para pedir uma palavra-passe sempre que o ficheiro for aberto. Escreva a palavra-passe duas vezes e clique em **Proteger**. O ficheiro é embaralhado ao ser guardado, para que ninguém o possa ler sem a palavra-passe. O JeffWrite usa o tipo de proteção forte que outros processadores de texto também utilizam, pelo que eles também podem abrir o ficheiro com a palavra-passe.

- Uma palavra-passe **não pode ser recuperada** se a esquecer. Guarde-a num local seguro.
- As versões que guardou e a cópia de segurança seguem a palavra-passe: ficam protegidas com ela e mudam quando a alterar ou remover.
- Para alterar a palavra-passe, clique novamente no mesmo cartão. Clique em **Remover Palavra-passe** para a retirar. A alteração fica guardada na próxima vez que guardar.
- As palavras-passe funcionam para ficheiros **.docx** e para ficheiros **.odt**. Um ficheiro .odt fica protegido da forma descrita pela norma OpenDocument, pelo que outros programas que sigam a norma também o podem abrir com a palavra-passe. Um ficheiro de texto simples ou .rtf não pode guardar uma palavra-passe, por isso o JeffWrite não guarda um documento protegido nesses formatos. Em vez de guardar sem a palavra-passe, informa o utilizador.

Para abrir um ficheiro protegido, abra-o como de costume. O JeffWrite pede a palavra-passe. Se não estiver correta, pode tentar novamente ou clicar em **Cancelar**.

O JeffWrite também abre um ficheiro **.doc** antigo que peça uma palavra-passe, e pede-a da mesma forma. O JeffWrite nunca guarda ficheiros .doc, por isso **Guardar Como** pede um novo nome, e a cópia .docx mantém a mesma palavra-passe.

## Inspecionar Documento

Clique em **Inspecionar Documento** antes de enviar um documento a alguém. A ferramenta indica o que o ficheiro contém e que poderá não querer partilhar, e quantos elementos de cada tipo encontrou:

- **Comentários** são notas que as pessoas deixaram na margem.
- **Alterações registadas** são edições que ainda não foram aceites nem rejeitadas. **Aceitar Tudo** mantém cada alteração como parte do texto.
- **Propriedades do documento e informações pessoais** são o autor, a pessoa que o guardou por último, a empresa e o gestor.
- **Texto oculto** é texto definido como oculto. Não é apresentado nem impresso, mas está no ficheiro.
- **Cabeçalhos e rodapés** são o texto no topo e na base das páginas.
- **Dados XML personalizados** são dados que outros programas acrescentaram ao ficheiro. Não são visíveis numa página.
- **Ligações externas** são ligações a outros ficheiros, e não a páginas Web: uma imagem ou um objeto apenas ligado (mostra um ficheiro que não está no documento), e um campo que obtém o texto ou a imagem de outro ficheiro (INCLUDEPICTURE, INCLUDETEXT, LINK, DDE e DDEAUTO). **Remover Tudo** transforma cada campo no texto que mostra neste momento, retira cada imagem ou objeto ligado e deixa a ligação de fora do ficheiro ao guardar. Os outros ficheiros não são alterados. As ligações em cabeçalhos, rodapés e notas de rodapé não são contadas, e as ligações Web mantêm-se.

Clique em **Remover Tudo** junto a um tipo para o retirar. Pode anular as alterações ao texto com **Ctrl+Z**. Guarde o documento depois para manter a cópia limpa.

**Remover informações pessoais deste ficheiro ao guardar**, por baixo dos cartões, faz isto sempre que guarda, para qualquer ficheiro, sem alterar o documento que tem aberto. É a mesma caixa que está em **Opções**, e é igual em todas as aplicações JeffOffice.

## Verificar Compatibilidade

Clique em **Verificar Compatibilidade** para ver o que um tipo de ficheiro mais antigo não consegue guardar. O JeffWrite guarda ficheiros **.docx** e **.odt**. Um ficheiro .docx guarda tudo o que está no documento. Um ficheiro .odt não consegue guardar algumas coisas, e esta lista indica quais. Se não aparecer nada, não se perde nada.

Ao guardar como ficheiro .odt, o JeffWrite lista o que o ficheiro não consegue guardar **antes** de escrever seja o que for. Pode escolher **Guardar Mesmo Assim**, **Guardar como .docx** (o ficheiro .docx é colocado ao lado, e nada se perde) ou **Cancelar**. Marque **Não voltar a perguntar para este ficheiro** e o JeffWrite deixa de perguntar sobre esse ficheiro até fechar o programa.

## Verificar Acessibilidade

Clique em **Verificar Acessibilidade** para abrir a lista de elementos que podem ser difíceis de ler ou utilizar para algumas pessoas, como uma imagem sem descrição. É a mesma verificação que **Rever > Verificar Acessibilidade**. Consulte [Contar palavras, definir o idioma, verificar a acessibilidade e procurar palavras](proofing-tools).

## Histórico de Versões

Clique em **Histórico de Versões** para guardar cópias com nome do documento no seu computador. Nada é enviado para lado nenhum. Uma versão é uma cópia completa do documento tal como está naquele momento. Guarde uma antes de uma alteração grande, para poder voltar a essa versão. O documento tem de estar guardado primeiro, porque as versões ficam guardadas com o nome do ficheiro.

- **Guardar uma Versão** pede um nome, como "Antes da edição", e guarda uma cópia.
- **Abrir uma Cópia** abre a versão escolhida numa nova janela. A própria versão não é alterada.
- **Comparar** coloca o documento e a versão escolhida lado a lado, como um só documento, com as diferenças apresentadas como alterações registadas. Consulte [Controlar alterações](track-changes).
- **Voltar a Esta Versão** torna o documento igual à versão escolhida. O JeffWrite guarda primeiro o documento tal como estava, como nova versão, para que nada se perca. Guarde o documento para manter a alteração.
- **Eliminar** remove a versão escolhida.

O JeffWrite guarda até 50 versões de um ficheiro e elimina as mais antigas quando houver mais.
