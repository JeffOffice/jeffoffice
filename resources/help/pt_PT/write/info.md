# Info: propriedades, palavras-passe e Inspecionar

<!-- keywords: verificar acessibilidade, informações, propriedades, título, autor, assunto, palavras-chave, categorias, comentários, empresa, gestor, propriedades personalizadas, docproperty, marcar como final, final, só de leitura, palavra-passe, cifrar, encriptar, proteger, palavra-passe de abertura, inspecionar documento, informações pessoais, texto oculto, xml personalizado, verificar compatibilidade, histórico de versões, versões, guardar uma versão, repor, restaurar, comparar versões, mostrar na pasta, copiar caminho, pasta, caminho -->

Clique em **Ficheiro > Informações** para ver os dados do seu documento e para o proteger antes de o partilhar. Prima **Esc** para regressar ao documento.

## Onde está o ficheiro

No topo, a página mostra onde o ficheiro está guardado. Clique em **Mostrar na Pasta** para abrir essa pasta. Clique em **Copiar Caminho** para copiar o caminho, para o poder colar num correio eletrónico ou numa conversa.

## Propriedades

As propriedades são dados sobre o documento, como o título e quem o escreveu. A página tem uma caixa para cada uma: **Título**, **Autor**, **Assunto**, **Palavras-chave**, **Categorias**, **Comentários**, **Empresa** e **Gestor**. Escreva numa caixa e prima **Tab** ou **Enter**. O JeffWrite mantém a alteração e guarda-a na próxima vez que guardar o documento.

Por baixo das caixas, a página mostra o que o JeffWrite calcula: quando o documento foi criado, guardado e impresso, e quantas páginas, palavras, carateres e parágrafos tem.

### As suas propriedades

Clique em **Propriedades Personalizadas** para criar propriedades próprias, como **Projeto** ou **Cliente**. Clique em **Adicionar**, escreva um nome, escolha o tipo de valor (texto, número inteiro, número, sim ou não, ou data) e escreva o valor.

Um campo pode mostrar uma propriedade personalizada. Clique em **Inserir > Campo** e escolha um campo. Altere o código para `DOCPROPERTY "Client"` e clique em **OK**. O texto mostra o valor. Prima **F9** para o atualizar depois de alterar a propriedade. Consulte [Campos](fields).

## Marcar como Final

Clique em **Marcar como Final** para indicar aos leitores que o documento está concluído. O JeffWrite guarda o documento e depois mantém-no apenas para leitura. Quando alguém o abre, a barra de título indica que é só de leitura, e uma nota informa que foi marcado como final. Esta é uma forma suave de dizer "por favor, não altere isto". Não é um bloqueio, porque qualquer pessoa pode escolher **Marcado como Final** novamente para o editar.

## Cifrar com Palavra-passe

Clique em **Cifrar com Palavra-passe** para pedir uma palavra-passe sempre que o ficheiro for aberto. Escreva a palavra-passe duas vezes e clique em **Proteger**. O ficheiro fica cifrado ao ser guardado, para que ninguém o possa ler sem a palavra-passe. O JeffWrite usa o tipo de proteção forte que outros processadores de texto também usam, para que estes possam abrir o ficheiro com a palavra-passe.

- Uma palavra-passe **não pode ser recuperada** se a esquecer. Guarde-a num local seguro.
- Para alterar a palavra-passe, clique novamente no mesmo cartão. Clique em **Remover Palavra-passe** para a retirar. A alteração fica guardada na próxima vez que guardar.
- As palavras-passe funcionam em ficheiros **.docx**. Um ficheiro **.odt** ainda não pode conter palavra-passe, por isso o JeffWrite não guarda um documento protegido como .odt. Em vez disso, informa-o, sem guardar sem palavra-passe.

Para abrir um ficheiro protegido, abra-o como de costume. O JeffWrite pede a palavra-passe. Se não estiver correta, pode tentar novamente ou clicar em **Cancelar**.

## Inspecionar Documento

Clique em **Inspecionar Documento** antes de enviar um documento a alguém. A ferramenta lista o que o ficheiro contém e que pode não querer partilhar, e quantos de cada tipo encontrou:

- **Comentários** são notas que as pessoas deixaram na margem.
- **Alterações registadas** são edições que ainda não foram aceites nem rejeitadas. **Aceitar Tudo** mantém cada alteração como parte do texto.
- **Propriedades do documento e informações pessoais** são o autor, a pessoa que guardou o documento pela última vez, a empresa e o gestor.
- **Texto oculto** é texto definido como oculto. Não é apresentado nem impresso, mas está no ficheiro.
- **Cabeçalhos e rodapés** são o texto no topo e na base das páginas.
- **Dados XML personalizados** são dados que outros programas acrescentaram ao ficheiro. Não são visíveis numa página.

Clique em **Remover Tudo** junto a um tipo para o retirar. Pode anular as alterações ao texto com **Ctrl+Z**. Guarde o documento depois para manter a cópia limpa.

## Verificar Compatibilidade

Clique em **Verificar Compatibilidade** para ver o que um tipo de ficheiro mais antigo não consegue guardar. O JeffWrite guarda ficheiros **.docx** e **.odt**. Um ficheiro .docx contém tudo o que está no documento. Um ficheiro .odt não consegue guardar algumas coisas, e esta lista indica quais. Se não aparecer nada, nada se perde.

Ao guardar como ficheiro .odt, o JeffWrite lista o que o ficheiro não consegue conter **antes** de escrever seja o que for. Pode escolher **Guardar como .odt Mesmo Assim**, **Guardar como .docx em Alternativa** (o ficheiro .docx é colocado ao lado, e nada se perde) ou **Cancelar**.

## Verificar Acessibilidade

Clique em **Verificar Acessibilidade** para abrir a lista de elementos que podem ser difíceis de ler ou utilizar para algumas pessoas, como uma imagem sem descrição. É a mesma verificação de **Rever > Verificar Acessibilidade**. Consulte [Contar palavras, definir o idioma, verificar acessibilidade e procurar palavras](proofing-tools).

## Histórico de Versões

Clique em **Histórico de Versões** para guardar cópias com nome do documento no seu computador. Nada é enviado para lado nenhum. Uma versão é uma cópia completa do documento tal como está naquele momento. Guarde uma antes de uma alteração grande, para poder voltar a ela. O documento tem de estar guardado primeiro, porque as versões são guardadas com o nome do ficheiro.

- **Guardar uma Versão** pede um nome, como "Antes da edição", e guarda uma cópia.
- **Abrir uma Cópia** abre a versão escolhida numa nova janela. A própria versão não é alterada.
- **Comparar** coloca o documento e a versão escolhida lado a lado como um único documento, com as diferenças apresentadas como alterações registadas. Consulte [Controlar alterações](track-changes).
- **Regressar a Esta Versão** torna o documento igual à versão escolhida. O JeffWrite guarda primeiro o documento tal como estava como uma nova versão, para que nada se perca. Guarde o documento para manter a alteração.
- **Eliminar** remove a versão escolhida.

O JeffWrite guarda até 50 versões de um ficheiro e elimina a mais antiga quando há mais do que isso.
