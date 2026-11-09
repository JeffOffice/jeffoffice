# Informações: propriedades, palavras-passe e Inspecionar

<!-- keywords: verificar acessibilidade, informações, propriedades, título, autor, assunto, palavras-chave, categorias, comentários, empresa, gestor, propriedades personalizadas, docproperty, marcar como final, final, só de leitura, palavra-passe, encriptar, proteger, palavra-passe de abertura, inspecionar documento, informações pessoais, texto oculto, xml personalizado, verificar compatibilidade, histórico de versões, versões, guardar uma versão, voltar atrás, restaurar, comparar versões, mostrar na pasta, copiar caminho, pasta, caminho -->

Clique em **Ficheiro > Informações** para ver os dados do seu documento e para o proteger antes de o partilhar. Prima **Esc** para voltar ao documento.

## Onde está o ficheiro

No topo, a página mostra onde o ficheiro está guardado. Clique em **Mostrar na Pasta** para abrir essa pasta. Clique em **Copiar Caminho** para copiar o caminho, de modo a poder colá-lo num correio eletrónico ou numa conversa.

## Propriedades

As propriedades são dados sobre o documento, como o título e quem o escreveu. A página tem uma caixa para cada uma: **Título**, **Autor**, **Assunto**, **Palavras-chave**, **Categorias**, **Comentários**, **Empresa** e **Gestor**. Escreva numa caixa e prima **Tab** ou **Enter**. O JeffWrite guarda a alteração e grava-a da próxima vez que guardar o documento.

Por baixo das caixas, a página mostra o que o JeffWrite calcula: quando o documento foi criado, guardado e impresso, e quantas páginas, palavras, carateres e parágrafos tem.

### As suas propriedades

Clique em **Propriedades Personalizadas** para criar propriedades próprias, como **Projeto** ou **Cliente**. Clique em **Adicionar**, escreva um nome, escolha o tipo de valor (texto, número inteiro, número, sim ou não, ou data) e escreva o valor.

Um campo pode mostrar uma propriedade personalizada. Clique em **Inserir > Campo** e escolha um campo. Altere o código para `DOCPROPERTY "Cliente"` e clique em **OK**. O texto mostra o valor. Prima **F9** para o atualizar depois de alterar a propriedade. Veja [Campos](fields).

## Marcar como Final

Clique em **Marcar como Final** para indicar aos leitores que o documento está concluído. O JeffWrite guarda o documento e depois mantém-no apenas para leitura. Quando alguém o abre, a barra de título indica que é só de leitura, e uma nota informa que foi marcado como final. Trata-se de uma forma suave de dizer «por favor, não altere isto». Não é um bloqueio, porque qualquer pessoa pode escolher **Marcado como Final** novamente para editar o documento.

## Encriptar com Palavra-passe

Clique em **Encriptar com Palavra-passe** para pedir uma palavra-passe sempre que o ficheiro for aberto. Escreva a palavra-passe duas vezes e clique em **Proteger**. O ficheiro fica cifrado quando o guardar, para que ninguém o possa ler sem a palavra-passe. O JeffWrite usa o tipo de proteção forte que outros processadores de texto também utilizam, pelo que estes também conseguem abrir o ficheiro com a palavra-passe.

- Uma palavra-passe **não pode ser recuperada** se a esquecer. Guarde-a num local seguro.
- As versões que guardou e a cópia de segurança seguem a palavra-passe: ficam protegidas por ela e mudam quando a alterar ou remover.
- Para alterar a palavra-passe, clique novamente no mesmo cartão. Clique em **Remover Palavra-passe** para a retirar. A alteração fica guardada da próxima vez que guardar.
- As palavras-passe funcionam para ficheiros **.docx**. Um ficheiro **.odt** ainda não pode conter uma palavra-passe, por isso o JeffWrite não guarda um documento protegido como .odt. Em vez disso, avisa-o, sem guardar sem a palavra-passe.

Para abrir um ficheiro protegido, abra-o como de costume. O JeffWrite pede a palavra-passe. Se não estiver correta, pode tentar novamente ou clicar em **Cancelar**.

## Inspecionar Documento

Clique em **Inspecionar Documento** antes de enviar um documento a alguém. A ferramenta lista o que o ficheiro contém e que pode não querer partilhar, e quantos itens de cada tipo encontrou:

- **Comentários** são notas que as pessoas deixaram na margem.
- **Alterações registadas** são edições que ainda não foram aceites nem rejeitadas. **Aceitar Tudo** mantém todas as alterações como parte do texto.
- **Propriedades do documento e informações pessoais** são o autor, a pessoa que o guardou por último, a empresa e o gestor.
- **Texto oculto** é texto definido como oculto. Não aparece nem é impresso, mas está no ficheiro.
- **Cabeçalhos e rodapés** são o texto no topo e na base das páginas.
- **Dados XML personalizados** são dados que outros programas acrescentaram ao ficheiro. Não os pode ver numa página.

Clique em **Remover Tudo** junto a um tipo para o retirar. Pode anular as alterações ao texto com **Ctrl+Z**. Guarde o documento depois para manter a cópia limpa.

## Verificar Compatibilidade

Clique em **Verificar Compatibilidade** para ver o que um tipo de ficheiro mais antigo não consegue conter. O JeffWrite guarda ficheiros **.docx** e **.odt**. Um ficheiro .docx contém tudo o que está no documento. Um ficheiro .odt não consegue conter algumas coisas, e esta lista indica quais. Se não aparecer nada, não se perde nada.

Ao guardar como ficheiro .odt, o JeffWrite lista o que o ficheiro não consegue conter **antes** de escrever seja o que for. Pode escolher **Guardar como .odt Mesmo Assim**, **Guardar como .docx em Alternativa** (o ficheiro .docx é criado ao lado, e nada se perde) ou **Cancelar**.

## Verificar Acessibilidade

Clique em **Verificar Acessibilidade** para abrir a lista de elementos que podem ser difíceis de ler ou utilizar para algumas pessoas, como uma imagem sem descrição. É a mesma verificação de **Revisão > Verificar Acessibilidade**. Veja [Contar palavras, definir o idioma, verificar a acessibilidade e pesquisar palavras](proofing-tools).

## Histórico de Versões

Clique em **Histórico de Versões** para guardar cópias com nome do documento no seu computador. Nada é enviado para lado nenhum. Uma versão é uma cópia completa do documento tal como está naquele momento. Guarde uma antes de uma grande alteração, para poder voltar a ela. O documento tem de estar guardado primeiro, porque as versões são guardadas com o nome do ficheiro.

- **Guardar uma Versão** pede um nome, como «Antes da edição», e guarda uma cópia.
- **Abrir uma Cópia** abre a versão escolhida numa nova janela. A própria versão não se altera.
- **Comparar** coloca o documento e a versão escolhida lado a lado, como um único documento, com as diferenças apresentadas como alterações registadas. Veja [Controlar alterações](track-changes).
- **Voltar a Esta Versão** torna o documento igual à versão escolhida. O JeffWrite guarda primeiro o documento tal como estava, como nova versão, para que nada se perca. Guarde o documento para manter a alteração.
- **Eliminar** remove a versão escolhida.

O JeffWrite guarda até 50 versões de um ficheiro e elimina as mais antigas quando há mais do que isso.
