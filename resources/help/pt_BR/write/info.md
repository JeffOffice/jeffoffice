# Informações: propriedades, senhas e Inspecionar

<!-- keywords: verificar acessibilidade, informações, propriedades, título, autor, assunto, palavras-chave, categorias, comentários, empresa, gerente, propriedades personalizadas, docproperty, marcar como final, final, somente leitura, senha, criptografar, proteger, senha para abrir, inspecionar documento, informações pessoais, texto oculto, xml personalizado, verificar compatibilidade, histórico de versões, versões, manter uma versão, voltar, restaurar, comparar versões, mostrar na pasta, copiar caminho, pasta, caminho -->

Clique em **Arquivo > Informações** para ver os dados do seu documento e protegê-lo antes de compartilhá-lo. Pressione **Esc** para voltar ao documento.

## Onde o arquivo está

No topo, a página mostra onde o arquivo está salvo. Clique em **Mostrar na Pasta** para abrir essa pasta. Clique em **Copiar Caminho** para copiar o caminho e colá-lo em um email ou em um chat.

## Propriedades

Propriedades são dados sobre o documento, como o título e quem o escreveu. A página tem uma caixa para cada uma: **Título**, **Autor**, **Assunto**, **Palavras-chave**, **Categorias**, **Comentários**, **Empresa** e **Gerente**. Digite na caixa e pressione **Tab** ou **Enter**. O JeffWrite mantém a alteração e a salva na próxima vez que você salvar o documento.

Abaixo das caixas, a página mostra o que o JeffWrite calcula: quando o documento foi criado, salvo e impresso, e quantas páginas, palavras, caracteres e parágrafos ele tem.

### Suas próprias propriedades

Clique em **Propriedades Personalizadas** para criar propriedades próprias, como **Projeto** ou **Cliente**. Clique em **Adicionar**, digite um nome, escolha o tipo de valor (texto, número inteiro, número, sim ou não, ou data) e digite o valor.

Um campo pode exibir uma propriedade personalizada. Clique em **Inserir > Campo** e escolha um campo. Altere o código para `DOCPROPERTY "Cliente"` e clique em **OK**. O texto mostra o valor. Pressione **F9** para atualizá-lo depois de alterar a propriedade. Veja [Campos](fields).

## Marcar como Final

Clique em **Marcar como Final** para informar aos leitores que o documento está concluído. O JeffWrite salva o documento e depois o mantém apenas para leitura. Quando alguém o abre, a barra de título indica que ele é somente leitura, e uma nota informa que ele foi marcado como final. É uma forma gentil de dizer "por favor, não altere isto". Não é um bloqueio, porque qualquer pessoa pode escolher **Marcada como Final** novamente para editá-lo.

## Criptografar com Senha

Clique em **Criptografar com Senha** para pedir uma senha sempre que o arquivo for aberto. Digite a senha duas vezes e clique em **Proteger**. O arquivo é embaralhado ao ser salvo, de modo que ninguém consegue lê-lo sem a senha. O JeffWrite usa o tipo de proteção forte que outros processadores de texto usam, então eles também podem abrir o arquivo com a senha.

- Uma senha **não pode ser recuperada** se você a esquecer. Guarde-a em um lugar seguro.
- As versões que você guardou e a cópia de backup seguem a senha: elas são protegidas por ela e mudam quando você altera ou remove a senha.
- Para alterar a senha, clique no mesmo cartão novamente. Clique em **Remover Senha** para retirá-la. A alteração é salva na próxima vez que você salvar.
- As senhas funcionam para arquivos **.docx** e para arquivos **.odt**. Um arquivo .odt é protegido da maneira descrita pelo padrão OpenDocument, de modo que outros programas que seguem o padrão também podem abri-lo com a senha. Um arquivo de texto simples ou .rtf não pode guardar senha, então o JeffWrite não salva um documento protegido nesses formatos. Em vez de salvar sem a senha, ele avisa você.

Para abrir um arquivo protegido, abra-o normalmente. O JeffWrite pede a senha. Se ela não estiver correta, você pode tentar novamente ou clicar em **Cancelar**.

O JeffWrite também abre um arquivo **.doc** antigo que pede senha. Ele pede da mesma forma. O JeffWrite nunca salva arquivos .doc, então **Salvar Como** pede um novo nome, e a cópia .docx mantém a mesma senha.

## Inspecionar Documento

Clique em **Inspecionar Documento** antes de enviar um documento a alguém. Ele lista o que o arquivo contém e que você talvez não queira compartilhar, e quantos itens de cada tipo encontrou:

- **Comentários** são notas que as pessoas deixaram na margem.
- **Alterações controladas** são edições que ainda não foram aceitas ou rejeitadas. **Aceitar Tudo** mantém cada alteração como parte do texto.
- **Propriedades do documento e informações pessoais** são o autor, a pessoa que salvou o arquivo por último, a empresa e o gerente.
- **Texto oculto** é texto configurado para ficar oculto. Ele não aparece nem é impresso, mas está no arquivo.
- **Cabeçalhos e rodapés** são os textos no topo e na base das páginas.
- **Dados XML personalizados** são dados que outros programas acrescentaram ao arquivo. Você não os vê em uma página.

Clique em **Remover Tudo** ao lado de um tipo para retirá-lo. Você pode desfazer as alterações no texto com **Ctrl+Z**. Salve o documento depois para manter a cópia limpa.

**Remover informações pessoais deste arquivo ao salvar**, abaixo dos cartões, faz isso em todas as vezes que você salvar, para qualquer arquivo, sem alterar o documento aberto. É a mesma caixa das **Opções**, e igual em todos os aplicativos do JeffOffice.

## Verificar Compatibilidade

Clique em **Verificar Compatibilidade** para ver o que um tipo mais antigo de arquivo não consegue guardar. O JeffWrite salva arquivos **.docx** e **.odt**. Um arquivo .docx guarda tudo o que há no documento. Um arquivo .odt não consegue guardar algumas coisas, e esta lista indica quais. Se nada aparecer na lista, nada será perdido.

Ao salvar como arquivo .odt, o JeffWrite lista o que o arquivo não consegue guardar **antes** de gravar qualquer coisa. Você pode escolher **Salvar Mesmo Assim**, **Salvar como .docx** (o arquivo .docx é salvo ao lado, e nada é perdido) ou **Cancelar**. Marque **Não perguntar novamente para este arquivo** e o JeffWrite deixa de perguntar sobre esse arquivo até você fechar o programa.

## Verificar Acessibilidade

Clique em **Verificar Acessibilidade** para abrir a lista de itens que podem ser difíceis de ler ou usar para algumas pessoas, como uma imagem sem descrição. É a mesma verificação de **Revisão > Verificar Acessibilidade**. Veja [Contar palavras, definir o idioma, verificar acessibilidade e pesquisar palavras](proofing-tools).

## Histórico de Versões

Clique em **Histórico de Versões** para guardar cópias nomeadas do documento no seu computador. Nada é enviado a lugar algum. Uma versão é uma cópia completa do documento como ele está naquele momento. Guarde uma antes de uma mudança grande, para poder voltar a ela. O documento precisa estar salvo antes, porque as versões são guardadas com o nome do arquivo.

- **Manter uma Versão** pede um nome, como "Antes da edição", e guarda uma cópia.
- **Abrir uma Cópia** abre a versão escolhida em uma nova janela. A própria versão não muda.
- **Comparar** coloca o documento e a versão escolhida lado a lado, como um único documento, com as diferenças mostradas como alterações controladas. Veja [Controlar alterações](track-changes).
- **Voltar a Esta Versão** deixa o documento igual à versão escolhida. O JeffWrite guarda antes o documento como estava, como uma nova versão, para que nada se perca. Salve o documento para manter a alteração.
- **Excluir** remove a versão escolhida.

O JeffWrite mantém até 50 versões de um arquivo e descarta as mais antigas quando há mais do que isso.
