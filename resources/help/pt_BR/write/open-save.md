# Abrir, salvar, imprimir e exportar
<!-- keywords: página da web, html, salvar como página da web, página da web filtrada, email, eml, indicadores, indicadores de PDF, shift+f5, última edição, onde você parou, modelos integrados, galeria, carta, relatório, resumo, manuscrito, boletim informativo, abrir, salvar, salvar como, docx, doc, dot, odt, opendocument, rtf, arquivos antigos, modelo, dotx, recentes, fixar, novo, fechar, imprimir, pdf, exportar, recuperar, recuperação automática, não salvo, travamento, tela de arquivo, formatos de arquivo, salvar uma cópia, somente leitura, abrir como cópia, nova janela, cópia de backup, meus modelos, salvar como modelo, ott, reparar, abrir e reparar, danificado -->

Clique em **Arquivo** no canto superior esquerdo para ver a tela Arquivo. Ela tem as páginas Novo, Abrir, Imprimir e Exportar, além dos comandos Salvar e Salvar Como. Pressione **Esc** para voltar ao seu documento.

## Iniciar um novo documento

Pressione **Ctrl+N** ou clique em **Arquivo > Novo** e depois em **Documento em Branco**. Um novo documento é aberto em sua própria janela. Se a janela em que você está estiver vazia e nunca tiver sido salva, o novo documento ocupa o lugar dela.

## Abrir um documento

Pressione **Ctrl+O** ou clique em **Arquivo > Abrir**. Clique em **Procurar** para escolher um arquivo. O JeffWrite abre estes tipos de arquivo:

- **.docx** é um documento.
- **.dotx** é um modelo. Um modelo é aberto como um novo documento feito a partir dele, então o modelo permanece como está.
- **.docm** e **.dotm** são um documento e um modelo que podem conter **macros**, pequenos programas que acompanham um arquivo. O JeffWrite nunca os executa, e os mantém como estavam quando você salva como .docm ou .dotm. Se você salvar como .docx ou .odt, as macros são deixadas de fora, e o JeffWrite avisa você.
- **.txt** é texto simples, apenas as palavras. Ao abri-lo, o JeffWrite o lê de imediato se ele estiver em ASCII simples ou Unicode. Se não estiver, o JeffWrite pergunta em qual **codificação** ler o arquivo e mostra as primeiras linhas, para que você veja se as letras estão corretas. Uma **codificação** é a forma como um arquivo armazena as letras, como UTF-8 ou Windows 1252. Ao salvar como .txt, você escolhe a codificação e o fim das linhas, e o arquivo guarda apenas as palavras: sem fontes, imagens ou linhas de tabela.
- **.html** e **.htm** são páginas da web. O JeffWrite lê as palavras, os títulos, o negrito, o itálico, o sublinhado, as cores, as listas e as tabelas. Os marcadores e os números das listas entram como marcas digitadas, e imagens e links são deixados de fora. Ele nunca busca nada na internet e ignora scripts. Uma página da web nunca é sobrescrita: **Salvar** pede um novo nome e salva um arquivo .docx. Para criar uma página da web, veja **Salvar como página da web** mais abaixo.
- **.doc** e **.dot** são os tipos mais antigos de documento e modelo. O JeffWrite os lê e salva o que você fizer com eles como um novo arquivo .docx.
- **.odt**, **.ott** e **.fodt** são arquivos OpenDocument. O JeffWrite lê os três. Ele salva arquivos .odt e salva o que você fizer com um arquivo .ott ou .fodt como um novo arquivo .odt. Veja [Abrir e salvar arquivos OpenDocument](open-odt).
- **.rtf** é Rich Text Format. O JeffWrite lê e salva esse formato. Veja [Abrir arquivos RTF](open-rtf).

A página **Abrir** também lista os documentos que você abriu ou salvou recentemente. Clique em um para abri-lo. Clique com o botão direito em um para fixá-lo no topo da lista ou removê-lo da lista.

Se um arquivo tiver partes que o JeffWrite ainda não consegue editar, como o layout de uma tabela ou um cabeçalho, ele as mantém. Veja [Bem-vindo ao JeffWrite](start). Se uma parte do arquivo não puder ser lida de jeito nenhum, o JeffWrite informa o que deixou de fora. Nesse caso ele não salva por cima desse arquivo: **Salvar** pede um novo nome, para que o original mantenha o que a cópia perdeu.

## Salvar um documento

Pressione **Ctrl+S** para salvar. Na primeira vez, o JeffWrite pede um nome. **Salvar Como** (F12) salva uma cópia com um novo nome. Escolha **Modelo (*.dotx)** para salvar um modelo, **Documento com Macros (*.docm)** para manter macros, **Texto Simples (*.txt)** para guardar apenas as palavras, ou **Rich Text Format (*.rtf)** para um arquivo que outros programas consigam ler. Um arquivo que veio de um tipo mais antigo é sempre salvo com um novo nome, para que o arquivo antigo nunca seja sobrescrito.

## Salvar uma cópia

Clique em **Arquivo > Salvar uma Cópia**. Escolha um nome. O JeffWrite grava o documento nesse arquivo, e você continua trabalhando no documento que tinha. O nome e as alterações não salvas dele permanecem como estavam. Isso é útil quando você quer enviar uma versão para alguém e continuar escrevendo.

## Manter uma cópia de backup

Clique em **Arquivo > Opções** e, na página **Geral**, ative **Sempre criar cópia de backup**. Cada vez que você salvar, o JeffWrite primeiro copia o arquivo antigo para outro chamado "Backup de" seguido do nome do arquivo, na mesma pasta. Se a cópia não puder ser feita, o JeffWrite pergunta se deve salvar sem ela. A resposta é Não, a menos que você diga Sim, e o arquivo continua como estava. Assim, você pode voltar à última versão salva.

## Salvar como modelo

Um modelo é um documento que você usa como ponto de partida. Clique em **Arquivo > Salvar como Modelo** e digite um nome. O modelo vai para **Meus Modelos**, e o documento que você está escrevendo não é alterado. Em **Salvar Como**, você também pode escolher **Modelo (*.dotx)** ou **Modelo de Texto OpenDocument (*.ott)** para salvar um arquivo de modelo onde preferir.

Para iniciar um novo documento a partir do seu modelo, clique em **Arquivo > Novo**. Seus modelos aparecem em **Meus Modelos**. Clique em um deles. Um novo documento é aberto com o texto e os estilos do modelo. Para remover um modelo da lista, clique com o botão direito nele e escolha **Remover de Meus Modelos**.

## Outras formas de abrir um arquivo

Na página **Abrir**, clique em **Mais Formas de Abrir**. Você também pode clicar com o botão direito em um nome da lista de Recentes. Cada forma pede um arquivo ou usa aquele em que você clicou:

- **Abrir Somente Leitura** abre o arquivo apenas para leitura. Você pode ver, rolar, pesquisar e copiar, mas não pode alterar nada. A barra de título mostra "Somente Leitura". Se você pressionar **Salvar**, o JeffWrite pede um novo nome, para que o arquivo aberto não seja alterado.
- **Abrir como Cópia** abre um novo documento com o texto do arquivo. Ele não tem arquivo próprio, então **Salvar** pergunta onde guardá-lo. O arquivo que você abriu nunca é alterado.
- **Abrir em Nova Janela** abre o arquivo em uma janela própria, mesmo que a janela em que você está esteja vazia.
- **Abrir e Reparar** serve para um arquivo .docx danificado, por exemplo um que foi interrompido ao ser copiado. Veja abaixo.

## Abrir e Reparar

Às vezes um arquivo .docx fica danificado. Uma cópia foi interrompida no meio, ou um disco cometeu erros. Um arquivo .docx é um conjunto de partes menores, como o texto, os estilos e as imagens. **Abrir e Reparar** lê cada parte que consegue. Uma parte interrompida no meio é mantida até o ponto em que parou. Uma parte que não pode ser lida é deixada de fora. Se a lista de partes ou as ligações entre elas estiverem perdidas, o JeffWrite as refaz.

O documento reparado é aberto como uma cópia com um nome como "Carta reparada". Uma mensagem informa o que foi mantido até uma interrupção e o que foi deixado de fora. O arquivo danificado não é alterado. Salve a cópia reparada com um nome próprio e confira-a antes de confiar nela.

## Recuperar trabalho não salvo

O JeffWrite salva uma cópia de reserva do trabalho não salvo a cada poucos minutos, como um arquivo em sua própria pasta. Se o programa fechar inesperadamente, ele oferece essas cópias na próxima vez que você o iniciar. Clique em **Arquivo > Abrir > Recuperar Trabalho Não Salvo** para vê-las depois. Você pode escolher quais abrir e salvar cada uma onde preferir.

## Começar a partir de um modelo

**Arquivo > Novo** mostra o **Documento em Branco** e um conjunto de modelos que acompanham o JeffWrite, agrupados por tipo: carta, relatório, resumo, manuscrito e boletim informativo. Cada imagem é a primeira página do documento que você recebe. Um **modelo** é um documento já preparado, com seus estilos, margens e textos de exemplo. Clique em um e substitua os textos de exemplo pelos seus. Os modelos que você mesmo salvou aparecem em **Meus Modelos**.

## Continuar de onde você parou

O JeffWrite lembra onde o cursor estava quando você salvou ou fechou um arquivo. Na próxima vez que você abrir o arquivo, o cursor estará ali. Pressione **Shift+F5** para ir aos lugares que você editou por último. Cada pressionamento vai ao anterior: os últimos três lugares e, depois, volta ao primeiro.

## Salvar como .odt

Um arquivo .odt não consegue guardar tudo o que um arquivo .docx guarda. Ao salvar como .odt, o JeffWrite primeiro lista o que seria perdido e permite salvar um arquivo .docx no lugar. Veja [Informações](info).

## Imprimir

Pressione **Ctrl+P** ou clique em **Arquivo > Imprimir**. Escolha uma impressora, as cópias e as páginas, veja cada página na visualização e clique em **Imprimir**. Veja [Imprimir](print).

## Criar um PDF

Clique em **Arquivo > Exportar > Exportar para PDF**. Escolha quais páginas incluir: todas, a página atual ou um intervalo como 1-3, 5 (p2s3 e s3 também funcionam aqui). Em seguida, escolha um nome. O PDF tem a mesma aparência em qualquer computador, porque as fontes ficam dentro dele. O título, o autor, o assunto e as palavras-chave do documento são gravados no PDF. Cada título vira um **indicador**, uma entrada na lista que um leitor de PDF mostra ao lado das páginas. Clique em um indicador para ir à página correspondente.

## Salvar como página da web

Clique em **Arquivo > Exportar** e escolha um dos dois tipos de página da web. Uma **página da web** é um arquivo que você pode abrir em qualquer navegador.

- **Salvar como Página da Web** cria um único arquivo .html que contém suas palavras, a aparência delas e suas imagens. Você pode enviar esse único arquivo sozinho. A aparência mantida é a fonte, o tamanho, a cor, o realce, o negrito, o itálico, o sublinhado, o alinhamento e o espaço entre parágrafos.
- **Salvar como Página da Web Filtrada** cria um pequeno arquivo .html apenas com a estrutura: títulos, negrito e itálico, listas, tabelas, links e imagens. Ele não tem fontes, tamanhos ou cores, então você pode definir a aparência depois.

Os dois mantêm títulos, listas com marcadores e numeradas, tabelas com células mescladas, links e imagens. Texto oculto, as palavras excluídas em uma alteração controlada, comentários, cabeçalhos, rodapés e notas são deixados de fora. Um link que iniciaria um programa ou executaria um script é salvo como texto simples.

## Criar um PDF/A

Na caixa **Exportar para PDF**, marque **PDF/A, para guardar por muito tempo**. Um arquivo PDF/A contém todas as fontes que usa e segue um padrão criado para guardar arquivos por muitos anos. Ele ainda não é marcado para leitores de tela e não tem senha.

## Criar um email

Clique em **Arquivo > Exportar > Criar um Email** para criar um arquivo de mensagem (.eml). Nada é enviado. Você escolhe o que vai na mensagem: o documento, o documento como PDF ou as páginas exibidas como imagens na mensagem (as primeiras 30 páginas). Abra o arquivo no seu programa de email, adicione o destinatário e envie.

## Relacionados

- [Bem-vindo ao JeffWrite](start)
- [Imprimir](print)
- [Usar o JeffWrite com o teclado](keyboard)
