# Info: propriedades, senhas e Inspecionar

<!-- keywords: verificar acessibilidade, informações, propriedades, título, autor, assunto, palavras-chave, categorias, comentários, empresa, gerente, propriedades personalizadas, docproperty, marcar como final, final, somente leitura, senha, criptografar, proteger, senha de abertura, inspecionar documento, informações pessoais, texto oculto, XML personalizado, verificar compatibilidade, histórico de versões, versões, manter uma versão, voltar, restaurar, comparar versões, mostrar na pasta, copiar caminho, pasta, caminho -->

Clique em **Arquivo > Informações** para ver os dados do seu documento e protegê-lo antes de compartilhá-lo. Pressione **Esc** para voltar ao documento.

## Onde está o arquivo

No alto da página, aparece o local onde o arquivo está salvo. Clique em **Mostrar na Pasta** para abrir essa pasta. Clique em **Copiar Caminho** para copiar o caminho e colá-lo em um email ou em um chat.

## Propriedades

Propriedades são informações sobre o documento, como o título e quem o escreveu. A página tem uma caixa para cada uma: **Título**, **Autor**, **Assunto**, **Palavras-chave**, **Categorias**, **Comentários**, **Empresa** e **Gerente**. Digite em uma caixa e pressione **Tab** ou **Enter**. O JeffWrite mantém a alteração e a salva na próxima vez que você salvar o documento.

Abaixo das caixas, a página mostra o que o JeffWrite calcula: quando o documento foi criado, salvo e impresso, e quantas páginas, palavras, caracteres e parágrafos ele tem.

### Suas próprias propriedades

Clique em **Propriedades Personalizadas** para criar propriedades próprias, como **Projeto** ou **Cliente**. Clique em **Adicionar**, digite um nome, escolha o tipo de valor (texto, número inteiro, número, sim ou não, ou data) e digite o valor.

Um campo pode mostrar uma propriedade personalizada. Clique em **Inserir > Campo** e escolha um campo. Altere o código para `DOCPROPERTY "Cliente"` e clique em **OK**. O texto mostra o valor. Pressione **F9** para atualizá-lo depois de alterar a propriedade. Veja [Campos](fields).

## Marcar como Final

Clique em **Marcar como Final** para informar aos leitores que o documento está concluído. O JeffWrite salva o documento e depois o mantém apenas para leitura. Quando alguém o abre, a barra de título indica que é somente leitura, e uma nota informa que ele foi marcado como final. É uma forma discreta de dizer "por favor, não altere isto". Não é um bloqueio, pois qualquer pessoa pode escolher **Marcada como Final** novamente para editar.

## Sempre Abrir como Somente Leitura

Clique em **Sempre Abrir como Somente Leitura** para que o documento seja aberto apenas para leitura sempre, evitando alterações acidentais. A escolha é salva com o arquivo quando você o salva. Na próxima vez que alguém abrir o arquivo, a barra de status indica que ele está aberto somente para leitura. Para editar, abra **Arquivo > Informações** e clique em **Sempre Abre como Somente Leitura**. É um único clique, então isto é um lembrete e não um bloqueio. Outros programas não leem essa escolha, e um arquivo .odt não pode armazená-la; por isso, salve primeiro o documento como .docx. Verificar Compatibilidade a indica para um arquivo .odt.

## Criptografar com Senha

Clique em **Criptografar com Senha** para exigir uma senha sempre que o arquivo for aberto. Digite a senha duas vezes e clique em **Proteger**. O arquivo é embaralhado quando você o salva, e ninguém consegue lê-lo sem a senha. O JeffWrite usa o tipo forte de proteção que outros processadores de texto também usam, de modo que eles também podem abrir o arquivo com a senha.

- Uma senha **não pode ser recuperada** se você a esquecer. Guarde-a em um lugar seguro.
- As versões que você manteve e a cópia de backup seguem a senha: elas são protegidas com ela e mudam quando você a altera ou remove.
- Para alterar a senha, clique novamente no mesmo cartão. Clique em **Remover Senha** para tirá-la. A alteração é salva na próxima vez que você salvar.
- As senhas funcionam para arquivos **.docx** e para arquivos **.odt**. Um arquivo .odt é protegido da forma descrita pelo padrão OpenDocument, de modo que outros programas que seguem o padrão também podem abri-lo com a senha. Um arquivo de texto simples ou .rtf não pode armazenar senha, por isso o JeffWrite não salvará um documento protegido nesses formatos. Ele avisa você em vez de salvar sem a senha.

Para abrir um arquivo protegido, abra-o normalmente. O JeffWrite pedirá a senha. Se ela não estiver correta, você pode tentar novamente ou clicar em **Cancelar**.

O JeffWrite também abre um arquivo **.doc** antigo que pede senha. Ele pede da mesma forma. O JeffWrite nunca salva um arquivo .doc, por isso **Salvar Como** pede um novo nome, e a cópia .docx mantém a mesma senha.

## Inspecionar Documento

Clique em **Inspecionar Documento** antes de enviar um documento a alguém. Ele lista o que o arquivo contém que você talvez não queira compartilhar, e quantos itens de cada tipo encontrou:

- **Comentários** são notas que as pessoas deixaram na margem.
- **Alterações controladas** são edições que ainda não foram aceitas ou rejeitadas. **Aceitar Tudo** mantém cada alteração como parte do texto.
- **Propriedades do documento e informações pessoais** são o autor, a pessoa que salvou o arquivo por último, a empresa e o gerente.
- **Texto oculto** é o texto definido como oculto. Ele não aparece nem é impresso, mas está no arquivo.
- **Cabeçalhos e rodapés** são os textos no topo e na base das páginas.
- **Dados de XML personalizado** são dados que outros programas acrescentaram ao arquivo. Você não os vê em uma página.
- **Links externos** são links para outros arquivos, não links da web: uma imagem ou um objeto que é apenas vinculado (mostra um arquivo que não está no documento), e um campo que obtém seu texto ou imagem de outro arquivo (INCLUDEPICTURE, INCLUDETEXT, LINK, DDE e DDEAUTO). **Remover Tudo** transforma cada campo no texto que ele mostra agora, retira cada imagem ou objeto vinculado e deixa o vínculo fora do arquivo ao salvar. Os outros arquivos não são alterados. Links em cabeçalhos, rodapés, notas de rodapé e notas de fim também são contados e removidos. Links da web permanecem. Links dentro de comentários, caixas de texto e partes que o programa mantém como estão (como o texto de desenhos) também são procurados. Uma imagem vinculada que também tenha sua própria cópia da imagem permanece, como imagem sem vínculo, e o vínculo é deixado fora do arquivo ao salvar. Remover links dentro de comentários e de outras partes mantidas como estão não pode ser desfeito; as caixas de texto podem ser desfeitas com um passo de Desfazer. O modelo ao qual o documento está anexado (seu caminho costuma conter um nome de usuário) também conta como link externo: **Remover Tudo** o desanexa, e o documento mantém seus próprios estilos. Outros vínculos para arquivos não são removidos, e uma linha abaixo da contagem os nomeia: a fonte de dados da mala direta, quadros, subdocumentos dentro de outras partes, dados externos de um gráfico, filmes e sons vinculados e folhas de estilo vinculadas. A mesma linha conta qualquer outro vínculo em um campo, comentário, caixa de texto ou na lista de vínculos do arquivo que nada no documento usa, e informa quando o endereço de uma imagem vinculada não está no arquivo ou quando uma parte está em uma codificação que a verificação não consegue ler. A linha então diz "Não verificado por completo" em vez de "Nenhum encontrado". Uma única execução de **Remover Tudo** retira no máximo 100.000 links nas partes mantidas; se restarem mais, a linha informa quantos, e você pode executar **Remover Tudo** novamente.

Clique em **Remover Tudo** ao lado de um tipo para removê-lo. Você pode desfazer as alterações no texto com **Ctrl+Z**. Links removidos de comentários e de outras partes mantidas, e dados de XML personalizado, não podem ser desfeitos. Salve o documento depois para manter a cópia limpa.

**Remover informações pessoais deste arquivo ao salvar**, abaixo dos cartões, faz isso toda vez que você salva, para qualquer arquivo, sem alterar o documento aberto. É a mesma caixa das **Opções**, e igual em todos os aplicativos do JeffOffice.

## Verificar Compatibilidade

Clique em **Verificar Compatibilidade** e escolha um tipo de arquivo para ver o que ele não pode armazenar. O JeffWrite salva arquivos **.docx**, **.odt**, **.rtf** e **.txt**. Um arquivo .docx armazena tudo o que há no seu documento. Os outros tipos não podem armazenar algumas coisas, e esta lista indica quais. Ela também informa as escolhas guardadas no próprio arquivo, Marcar como Final e Sempre Abrir como Somente Leitura, quando estiverem ativas, e a senha para um tipo que não pode armazenar uma. Se nada aparecer na lista, nada será perdido.

Ao salvar como um arquivo .odt, o JeffWrite lista o que o arquivo não pode armazenar **antes** de gravar qualquer coisa. Você pode escolher **Salvar Mesmo Assim**, **Salvar como .docx** (o arquivo .docx é criado ao lado dele, e nada é perdido) ou **Cancelar**. Marque **Não perguntar novamente para este arquivo** e o JeffWrite deixa de perguntar sobre esse arquivo até que você feche o programa.

## Verificar Acessibilidade

Clique em **Verificar Acessibilidade** para abrir a lista de itens que podem ser difíceis de ler ou usar para algumas pessoas, como uma imagem sem descrição. É a mesma verificação de **Revisão > Verificar Acessibilidade**. Veja [Contar palavras, definir o idioma, verificar acessibilidade e pesquisar palavras](proofing-tools).

## Histórico de Versões

Clique em **Histórico de Versões** para manter cópias nomeadas do documento no seu computador. Nada é enviado a lugar nenhum. Uma versão é uma cópia completa do documento tal como está naquele momento. Mantenha uma antes de uma grande alteração, para poder voltar a ela. O documento precisa estar salvo antes, porque as versões são guardadas sob o nome do arquivo.

- **Manter uma Versão** pede um nome, como "Antes da edição", e guarda uma cópia.
- **Abrir uma Cópia** abre a versão escolhida em uma nova janela. A própria versão não muda.
- **Comparar** coloca o documento e a versão escolhida lado a lado, como um único documento, com as diferenças mostradas como alterações controladas. Veja [Controlar alterações](track-changes).
- **Voltar a Esta Versão** torna o documento igual à versão escolhida. O JeffWrite mantém antes o documento como estava, como uma nova versão, para que nada seja perdido. Salve o documento para manter a alteração.
- **Excluir** remove a versão escolhida.

O JeffWrite mantém até 50 versões de um arquivo e descarta as mais antigas quando há mais do que isso.
